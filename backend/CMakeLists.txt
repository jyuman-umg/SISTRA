add_executable(SISTRA
        src/main.cpp
)

target_compile_features(SISTRA
        PRIVATE
        cxx_std_20
)