#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os


def execute(cmd):
    print('>> ' + cmd)
    os.system(cmd)


def init():
    execute("pip3 install --upgrade conan")
    execute("conan remote add -f eliasku https://api.bintray.com/conan/eliasku/public-conan")
    execute("conan install -if cmake-build-debug -pr clion-debug -b missing .")


def create_local_b(target_path, ref, build_type="Debug", build_profile="default", arch=None):
    install_folder = "builds/_deps/%s/%s" % (target_path, build_type)

    arg_arch = f"-s arch={arch}" if arch else ''
    print("ARCH: %s" % arg_arch)

    execute("conan install -s build_type=%s -pr %s %s -if %s -b outdated %s" %
            (build_type, build_profile, arg_arch, install_folder, target_path))

    execute("conan create -s build_type=%s -pr %s %s %s %s" %
            (build_type, build_profile, arg_arch, target_path, ref))


def create_local(target_path, ref, build_profile="default", arch=None):
    create_local_b(target_path, ref, "Debug", build_profile, arch)
    create_local_b(target_path, ref, "Release", build_profile, arch)


init()

#create_local(".", "ekx/0.0.1@eliasku/testing")
create_local_b(".", "ekx/0.0.1@eliasku/testing", "Release", "wasm-release")
