#pragma once

#include <jni.h>
#include <stdint.h>

namespace zygisk {

enum Option : uint32_t {
    FORCE_DENYLIST_UNSHARE = 1 << 0,
    DLCLOSE_MODULE_LIBRARY = 1 << 1,
};

struct AppSpecializeArgs {
    jint &uid;
    jint &gid;
    jint_array &gids;
    jint &runtime_flags;
    jobject_array &rlimits;
    jint &mount_external;
    jstring &se_info;
    jstring &nice_name;
    jboolean &is_child_zygote;
    jstring &instruction_set;
    jstring &app_data_dir;
    jboolean &is_top_app;
    jobject_array &pkg_data_info_list;
    jobject_array &whitelisted_data_info_list;
    jboolean &mount_data_dirs;
    jboolean &use_legacy_data;
};

struct ServerSpecializeArgs {
    jint &uid;
    jint &gid;
    jint_array &gids;
    jint &runtime_flags;
    jobject_array &rlimits;
    jlong &permitted_capabilities;
    jlong &effective_capabilities;
};

class Api {
public:
    virtual int getApiVersion() = 0;
    virtual void setOption(Option opt) = 0;
};

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

}

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" __attribute__((visibility("default"))) \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    module.onLoad(api, env); \
}

