#include <android/log.h>
#include <unistd.h>
#include <string>
#include <dlfcn.h>
#include <pthread.h>
#include "zygisk.hpp"

#define LOG_TAG "CODM_Optimization"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#define TARGET_PACKAGE "com.garena.game.codm"
#define ENGINE_LIB "libil2cpp.so"

class CodmZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *zygisk_api, JNIEnv *jni_env) override {
        this->api = zygisk_api;
        this->env = jni_env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        const char *process_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process_name) {
            std::string pkg(process_name);
            env->ReleaseStringUTFChars(args->nice_name, process_name);

            if (pkg == TARGET_PACKAGE) {
                LOGD("CODM Garena target recognized. Securing process memory space.");
                api->setOption(zygisk::Option::FORCE_DENYLIST_UNSHARE);
            }
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        pthread_t tid;
        pthread_create(&tid, nullptr, injection_worker, nullptr);
    }

private:
    zygisk::Api *api;
    JNIEnv *env;

    static void* injection_worker(void*) {
        void* handle = nullptr;
        while (handle == nullptr) {
            handle = dlopen(ENGINE_LIB, RTLD_NOLOAD);
            if (!handle) handle = dlopen("libAnort.so", RTLD_NOLOAD);
            usleep(200000); 
        }

        LOGD("Target engine binary mapped. Deploying function hooks...");
        return nullptr;
    }
};

REGISTER_ZYGISK_MODULE(CodmZygiskModule)
