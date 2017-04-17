{
    "variables": {
        "swipl%":"<!(node swipl_home.js)"
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
                "<(swipl)/include",
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
                    "libraries": [ "-l<(swipl)/lib/libswipl.lib" ]
                },{
                    "libraries": [ "-lswipl -L<(swipl)/lib/x86_64-linux -Wl,-rpath <(swipl)/lib/x86_64-linux" ]
                }]
            ]
        }
    ]
}
