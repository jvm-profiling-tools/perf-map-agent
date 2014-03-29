#include <jni.h>
#include <jvmti.h>
#include "jvmticmlr.h"

FILE *method_file = NULL;

void JNICALL
cbVMInit(jvmtiEnv *jvmti_env,
            JNIEnv* jni_env,
            jthread thread) {
    if (!method_file)
        open_file();
}

void open_file() {
    char methodFileName[500];
    sprintf(methodFileName, "/tmp/perf-%d.map", getpid());
    method_file = fopen(methodFileName, "w");
}

static void JNICALL
cbVMStart(jvmtiEnv *jvmti, JNIEnv *env) {
    jvmtiJlocationFormat format;
    (*jvmti)->GetJLocationFormat(jvmti, &format);
    //printf("[tracker] VMStart LocationFormat: %d\n", format);
}

static int get_line_number(jvmtiLineNumberEntry *table, jint entry_count, jlocation loc) {
  int i;
  for (i = 0; i < entry_count; i++)
    if (table[i].start_location > loc) return table[i - 1].line_number;

  return -1;
}

static void sig_string(jvmtiEnv *env, jmethodID method, char *output, int noutput) {
    char *name;
    char *msig;
    jclass class;
    char *csig;

    (*env)->GetMethodName(env, method, &name, &msig, NULL);
    (*env)->GetMethodDeclaringClass(env, method, &class);
    (*env)->GetClassSignature(env, class, &csig, NULL);

    snprintf(output, noutput, "%s.%s%s", csig, name, msig);

    (*env)->Deallocate(env, name);
    (*env)->Deallocate(env, msig);
    (*env)->Deallocate(env, csig);
}

static void JNICALL
cbCompiledMethodLoad(jvmtiEnv *env,
            jmethodID method,
            jint code_size,
            const void* code_addr,
            jint map_length,
            const jvmtiAddrLocationMap* map,
            const void* compile_info) {
    int i;

    char name_buffer[1000];
    sig_string(env, method, name_buffer, 1000);

    jvmtiCompiledMethodLoadRecordHeader *header = compile_info;
    //printf("[tracker] Loaded %s addr: %lx to %lx length: %d with %d location entries, header has kind: %d\n", name_buffer, code_addr, code_addr + code_size, code_size, map_length, header->kind);
    if (header->kind == JVMTI_CMLR_INLINE_INFO) {
        jvmtiCompiledMethodLoadInlineRecord *record = header;
        //printf("[inline] Got %d PC records\n", record->numpcs);

        void *start_addr = code_addr;
        jmethodID cur_method = method;
        char name2_buffer[1000];
        for (i = 0; i < record->numpcs; i++) {
            PCStackInfo *info = &record->pcinfo[i];
            jmethodID top_method = info->methods[0];
            if (cur_method != top_method) {
                void *end_addr = info->pc;
                
                if (top_method != method) {
                    sig_string(env, top_method, name2_buffer, 1000);
                    fprintf(method_file, "%lx %x %s in %s\n", start_addr, end_addr - start_addr, name2_buffer, name_buffer);
                } else
                    fprintf(method_file, "%lx %x %s\n", start_addr, end_addr - start_addr, name_buffer);

                start_addr = info->pc;
                cur_method = top_method;
            }
        }
        if (start_addr != code_addr + code_size) {
            void *end_addr = code_addr + code_size;
            sig_string(env, cur_method, name2_buffer, 1000);
            fprintf(method_file, "%lx %x %s in %s\n", start_addr, end_addr - start_addr, name2_buffer, name_buffer);
        }
    } else
      fprintf(method_file, "%lx %x %s\n", code_addr, code_size, name_buffer);
    
    fsync(fileno(method_file));
}

void JNICALL
cbDynamicCodeGenerated(jvmtiEnv *jvmti_env,
            const char* name,
            const void* address,
            jint length) {
    if (!method_file)
        open_file();

    fprintf(method_file, "%lx %x %s\n", address, length, name);
    //printf("[tracker] Code generated: %s %lx %x\n", name, address, length);
}

void JNICALL
cbCompiledMethodUnload(jvmtiEnv *jvmti_env,
            jmethodID method,
            const void* code_addr) {
    //printf("[tracker] Unloaded %ld code_addr: 0x%lx\n", method, code_addr);
}

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    jvmtiEnv              *jvmti;
    jvmtiError             error;
    jint                   res;
    jvmtiCapabilities      capabilities;
    jvmtiEventCallbacks    callbacks;

    // Create the JVM TI environment (jvmti).
    res = (*vm)->GetEnv(vm, (void **)&jvmti, JVMTI_VERSION_1);
    // If res!=JNI_OK generate an error.

    // Parse the options supplied to this agent on the command line.
    //parse_agent_options(options);
    // If options don't parse, do you want this to be an error?

    // Clear the capabilities structure and set the ones you need.
    (void)memset(&capabilities,0, sizeof(capabilities));
    capabilities.can_generate_all_class_hook_events  = 1;
    capabilities.can_tag_objects                     = 1;
    capabilities.can_generate_object_free_events     = 1;
    capabilities.can_get_source_file_name            = 1;
    capabilities.can_get_line_numbers                = 1;
    capabilities.can_generate_vm_object_alloc_events = 1;
    capabilities.can_generate_compiled_method_load_events = 1;

    // Request these capabilities for this JVM TI environment.
    error = (*jvmti)->AddCapabilities(jvmti, &capabilities);
    // If error!=JVMTI_ERROR_NONE, your agent may be in trouble.

    // Clear the callbacks structure and set the ones you want.
    (void)memset(&callbacks,0, sizeof(callbacks));
    callbacks.VMInit           = &cbVMInit;
    callbacks.VMStart           = &cbVMStart;
    callbacks.CompiledMethodLoad  = &cbCompiledMethodLoad;
    callbacks.CompiledMethodUnload  = &cbCompiledMethodUnload;
    callbacks.DynamicCodeGenerated = &cbDynamicCodeGenerated;
    error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks,
                      (jint)sizeof(callbacks));
    //  If error!=JVMTI_ERROR_NONE, the callbacks were not accepted.

    // For each of the above callbacks, enable this event.
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_VM_INIT, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_VM_START, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_COMPILED_METHOD_LOAD, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_COMPILED_METHOD_UNLOAD, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_DYNAMIC_CODE_GENERATED, (jthread)NULL);
    // In all the above calls, check errors.

    return JNI_OK; // Indicates to the VM that the agent loaded OK.
}


