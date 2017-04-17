{
    "variables": {
        "plbase%":"<!(node swipl_home.js PLBASE)",
        "plarch%":"<!(node swipl_home.js PLARCH)"
    },
    "targets": [
        {
            "target_name": "swipl",
            "product_prefix": "lib",
            "sources": [
                "./src/libswipl.cc"
            ],
            "include_dirs": [
                "./src",
                "<(plbase)/include",
                "<!(node -e \"require('nan')\")"
            ],
            "direct_dependent_settings": {
                "linkflags": [
                    "-D_FILE_OFFSET_BITS=64",
                    "-D_LARGEFILE_SOURCE"
                ]
            },
            "conditions": [
                ['OS=="win"', {
                    "libraries": [ "-l<(plbase)/lib/libswipl.lib" ]
                },{
                    "libraries": [ "-lswipl -L<(plbase)/lib/<(plarch) -Wl,-rpath <(plbase)/lib/<(plarch)" ]
                }]
            ]
        }
    ]
}
