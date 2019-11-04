from conans import ConanFile, CMake, tools


class EKX_Conan(ConanFile):
    name = "ekx"
    version = "0.0.1"
    license = "ISC"
    author = "eliasku deadbabe@gmail.com"
    url = "https://gitlab.com/eliasku/ek-common"
    description = "base platform library"
    settings = "os", "compiler", "arch", "build_type"
    # options = {"editor": [True, False]}
    # default_options = {"editor": True}
    generators = "cmake"
    exports_sources = [
        "CMakeLists.txt",
        "core*",
        "ecxx*",
        "flash*",
        "ek*",
        "scenex*",
        "editor*"
    ]
    no_copy_source = True

    _conan_run_tests = False
    _conan_run_benchmarks = False
    _conan_build_coverage = False

    requires = [
        # FLASH tooling
        "pugixml/1.9@bincrafters/stable",  # upgrade to 1.10
        "cairo/1.17.2@bincrafters/stable",

        # EDITOR tooling
        "kainjow-mustache/3.2.1@bincrafters/stable",
        "jsonformoderncpp/3.7.0@vthiery/stable",
        "fmt/5.3.0@bincrafters/stable",
        "imgui/1.73@bincrafters/stable"
    ]

    def configure(self):
        self._conan_run_tests = tools.get_env("CONAN_RUN_TESTS", True)
        self._conan_build_coverage = self.settings.get_safe("build_type") == "Debug" and \
                                     self._conan_run_tests and \
                                     tools.get_env("CONAN_BUILD_COVERAGE", False)

    def build_requirements(self):
        if self._conan_run_tests:
            self.output.highlight("conan_run_tests")
            self.build_requires("gtest/1.8.1@bincrafters/stable")
        if self._conan_run_benchmarks:
            self.build_requires("entt/3.2.0@skypjack/stable")

    def _configure_cmake(self):
        defs = {'BUILD_TESTS': self._conan_run_tests,
                'BUILD_COVERAGE': self._conan_build_coverage,
                'BUILD_BENCHMARKS': self._conan_run_benchmarks}

        if tools.get_env("CMAKE_SKIP_COMPILER_CHECKS", True):
            defs['CMAKE_C_COMPILER_FORCED'] = 'TRUE'
            defs['CMAKE_CXX_COMPILER_FORCED'] = 'TRUE'

        cmake = CMake(self)
        cmake.configure(defs=defs)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self._conan_run_tests:
            cmake.test(output_on_failure=True)

    def package(self):
        self.copy("*.cpp", dst="src", src="core/src/")
        self.copy("*.h", dst="include", src="core/src/")
        self.copy("*.hpp", dst="include", src="core/src/")

        self.copy("*.cpp", dst="src", src="ecxx/src/")
        self.copy("*.h", dst="include", src="ecxx/src/")
        self.copy("*.hpp", dst="include", src="ecxx/src/")
        self.copy("*.cpp", dst="src", src="editor/src/")
        self.copy("*.h", dst="include", src="editor/src/")
        self.copy("*.hpp", dst="include", src="editor/src/")

        self.copy("*.cpp", dst="src", src="scenex/src/")
        self.copy("*.h", dst="include", src="scenex/src/")
        self.copy("*.hpp", dst="include", src="scenex/src/")

        self.copy("*.cpp", dst="src", src="flash/src/")
        self.copy("*.h", dst="include", src="flash/src/")
        self.copy("*.hpp", dst="include", src="flash/src/")

        self.copy("*.cpp", dst="src", src="ek/src/")
        self.copy("*.h", dst="include", src="ek/src/")
        self.copy("*.hpp", dst="include", src="ek/src/")

        self.copy("*mylib.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

        self.copy("license*", dst="licenses", ignore_case=True, keep_path=False)

        try:
            cmake = self._configure_cmake()
            cmake.install()
        except:
            self.output.warn("No CMake install target")

    def package_info(self):
        # self.deps = {"ek-core": [], "ek": ["ek-core"], "scenex": ["ek-core", "ecxx"], "ecxx": []}
        # self.cpp_info.ekx.libs = ["ek-core", "ek", "scenex", "ecxx"]
        # self.cpp_info.ekx.srcdirs = ["core/src", "ek/src", "scenex/src", "ecxx/src"]
        # self.cpp_info.ekxe.libs = ["ek-core", "ek", "scenex", "ecxx", "ek-flash", "ek-editor"]
        # self.cpp_info.ekxe.srcdirs = ["core/src", "ek/src", "scenex/src", "ecxx/src", "flash/src", "editor/src"]
        self.cpp_info.libs = ["ek-core", "ek", "scenex", "ecxx", "ek-flash", "ek-editor"]
        # self.cpp_info.srcdirs = ["core/src", "ek/src", "scenex/src", "ecxx/src", "flash/src", "editor/src"]
        # self.cpp_info.libdirs = ["core/src", "ek", "scenex/src/scenex", "ecxx/src", "flash/src", "editor/src"]
        self.cpp_info.includedirs = []
