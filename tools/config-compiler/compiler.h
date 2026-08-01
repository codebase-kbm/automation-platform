#ifndef AP_CONFIG_COMPILER_H
#define AP_CONFIG_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

int ap_config_compile(
    const char *input_file,
    const char *output_file
);

#ifdef __cplusplus
}
#endif

#endif /* AP_CONFIG_COMPILER_H */