#pragma once

#include "TextureLoader.hpp"
#include <jni.h>
#include <ek/app/Platform.h>

namespace ek {

void TextureLoader::load() {
    loading = true;
    status = 0;
    progress = 0.0f;
    const char *urls_[IMAGES_MAX_COUNT]{};
    for (int i = 0; i < IMAGES_MAX_COUNT; ++i) {
        urls_[i] = urls[i].c_str();
    }
    int flags = 0;
    if (premultiplyAlpha) flags |= 1;
    if (isCubeMap) flags |= 2;

    auto *env = app::getJNIEnv();
    jobject jBasePath = nullptr;
    if (!basePath.empty()) {
        jBasePath = env->NewStringUTF(basePath.c_str());
    }
    jobjectArray jURLs = env->NewObjectArray(imagesToLoad, env->FindClass("java/lang/String"),
                                             0);
    for (int i = 0; i < imagesToLoad; ++i) {
        jstring str = env->NewStringUTF(urls_[i]);
        env->SetObjectArrayElement(jURLs, i, str);
    }
    auto cls = env->FindClass("ek/TextureLoader");

    auto method = env->GetStaticMethodID(cls, "load",
                                         "(Landroid/content/Context;Ljava/lang/String;[Ljava/lang/String;II)I");
    handle = env->CallStaticIntMethod(cls, method, app::get_context(), jBasePath, jURLs, flags, formatMask);

    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(jURLs);
    if (jBasePath) {
        env->DeleteLocalRef(jBasePath);
    }
}

void TextureLoader::update() {
    if (imagesToLoad <= 0 || !loading) {
        return;
    }
    auto *env = app::getJNIEnv();
    auto cls = env->FindClass("ek/TextureLoader");

    auto metGetProgress = env->GetStaticMethodID(cls, "getProgress",
                                                 "(I)I");
    const auto prog = env->CallStaticIntMethod(cls, metGetProgress, handle);

    progress = prog / 100.0f;
    if (prog >= 100) {
        auto metGetWidth = env->GetStaticMethodID(cls, "getWidth", "(I)I");
        auto metGetHeight = env->GetStaticMethodID(cls, "getHeight", "(I)I");
        auto metGetTexture = env->GetStaticMethodID(cls, "getTexture", "(I)I");
        auto metDestroy = env->GetStaticMethodID(cls, "destroy", "(I)I");
        const auto width = env->CallStaticIntMethod(cls, metGetWidth, handle);
        const auto height = env->CallStaticIntMethod(cls, metGetHeight, handle);
        const auto texture = env->CallStaticIntMethod(cls, metGetTexture, handle);
        desc.type = isCubeMap ? SG_IMAGETYPE_CUBE : SG_IMAGETYPE_2D;
        desc.width = width;
        desc.height = height;
        desc.usage = SG_USAGE_IMMUTABLE;
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        desc.gl_textures[0] = texture;
        desc.gl_texture_target = isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        image = sg_make_image(&desc);

        //const auto result =
        env->CallStaticIntMethod(cls, metDestroy, handle);

        handle = 0;
        progress = 1.0f;
        loading = false;
        status = 0;
    }

    env->DeleteLocalRef(cls);
}

}