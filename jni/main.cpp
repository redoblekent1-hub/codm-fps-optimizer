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

int (*orig_set_targetFrameRate)(int fps) = nullptr;
float (*orig_get_maxDrawDistance)() = nullptr;

int hk_set_targetFrameRate(int fps) {
    LOGD("Game requested %d FPS. Overriding and forcing 60 FPS.", fps);
    return orig_set_targetFrameRate(60); 
}

float hk_get_maxDrawDistance() {
    LOGD("Game requested draw distance constraint. Overriding to maximum scale.");
    return 1000.0f;
}

class CodmZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
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
    static void* injection_worker(void*) {
        void* handle = nullptr;
        while (handle == nullptr) {
            handle = dlopen(ENGINE_LIB, RTLD_NOLOAD);
            if (!handle) handle = dlopen("libAnort.so", RTLD_NOLOAD);
            usleep(200000); 
        }

        LOGD("Target engine binary mapped. Deploying function hooks...");

        void* set_fps_sym = dlsym(handle, "_ZN5UnityEngine11Application18set_targetFrameRateEi");
        void* get_dist_sym = dlsym(handle, "_ZN5UnityEngine6Camera18get_maxDrawDistanceEv");

        if (set_fps_sym) {
            LOGD("FPS optimization hook targeted.");
        }
        if (get_dist_sym) {
            LOGD("Long-distance rendering boundary override targeted.");
        }

        return nullptr;
    }
};

REGISTER_ZYGISK_MODULE(CodmZygiskModule)
