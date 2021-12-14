#include <ek/texture_loader.h>
#include <ek/app_native.h>
#include <GLES2/gl2.h>

void ek_texture_loader_load(ek_texture_loader* loader) {
   loader->loading = true;
   loader->status = 0;
   loader->progress = 0.0f;
    int flags = 0;
    if (loader->premultiplyAlpha) flags |= 1;
    if (loader->isCubeMap) flags |= 2;

    JNIEnv *env = ek_android_jni();
    jobject jBasePath = 0;
    if (loader->basePath.path[0] != '\0') {
        jBasePath = (*env)->NewStringUTF(env, loader->basePath.path);
    }
    jobjectArray jURLs = (*env)->NewObjectArray(env, loader->imagesToLoad, (*env)->FindClass(env, "java/lang/String"),
                                             0);
    for (int i = 0; i < loader->imagesToLoad; ++i) {
        jstring str = (*env)->NewStringUTF(env, loader->urls[i].path);
        (*env)->SetObjectArrayElement(env, jURLs, i, str);
    }
    jclass cls = (*env)->FindClass(env, "ek/TextureLoader");

    jmethodID method = (*env)->GetStaticMethodID(env, cls, "load",
                                         "(Landroid/content/Context;Ljava/lang/String;[Ljava/lang/String;II)I");
    loader->handle = (*env)->CallStaticIntMethod(env, cls, method, ek_android_context(), jBasePath, jURLs, flags, (int)loader->formatMask);

    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, jURLs);
    if (jBasePath) {
        (*env)->DeleteLocalRef(env, jBasePath);
    }
}

void ek_texture_loader_update(ek_texture_loader* loader) {
    if (loader->imagesToLoad <= 0 || !loader->loading) {
        return;
    }
    JNIEnv *env = ek_android_jni();
    jclass cls = (*env)->FindClass(env, "ek/TextureLoader");

    jmethodID metGetProgress = (*env)->GetStaticMethodID(env, cls, "getProgress",
                                                 "(I)I");
    const jint prog = (*env)->CallStaticIntMethod(env, cls, metGetProgress, loader->handle);

    loader->progress = (float)prog / 100.0f;
    if (prog >= 100) {
        jmethodID metGetWidth = (*env)->GetStaticMethodID(env, cls, "getWidth", "(I)I");
        jmethodID metGetHeight = (*env)->GetStaticMethodID(env, cls, "getHeight", "(I)I");
        jmethodID metGetTexture = (*env)->GetStaticMethodID(env, cls, "getTexture", "(I)I");
        jmethodID metDestroy = (*env)->GetStaticMethodID(env, cls, "destroy", "(I)I");
        const jint width = (*env)->CallStaticIntMethod(env, cls, metGetWidth, loader->handle);
        const jint height = (*env)->CallStaticIntMethod(env, cls, metGetHeight, loader->handle);
        const jint texture = (*env)->CallStaticIntMethod(env, cls, metGetTexture, loader->handle);
        loader->desc.type = loader->isCubeMap ? SG_IMAGETYPE_CUBE : SG_IMAGETYPE_2D;
        loader->desc.width = width;
        loader->desc.height = height;
        loader->desc.usage = SG_USAGE_IMMUTABLE;
        loader->desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        loader->desc.min_filter = SG_FILTER_LINEAR;
        loader->desc.mag_filter = SG_FILTER_LINEAR;
        loader->desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        loader->desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        loader->desc.gl_textures[0] = texture;
        loader->desc.gl_texture_target = loader->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        loader->image = sg_make_image(&loader->desc);

        //const auto result =
        (*env)->CallStaticIntMethod(env, cls, metDestroy, loader->handle);

        loader->handle = 0;
        loader->progress = 1.0f;
        loader->loading = false;
        loader->status = 0;
    }

    (*env)->DeleteLocalRef(env, cls);
}
