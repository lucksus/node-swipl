{
  "targets": [
    {
      "target_name": "swipl",
      "product_prefix": "lib",
      "sources": [
        "./src/libswipl.cc"
      ],
      "include_dirs": [
        "./src",
        "$(SWI_HOME_DIR)/include",
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
            "libraries": [
              "-l$(SWI_HOME_DIR)/lib/libswipl.lib"
            ]
          },{
            "libraries": [
              "-lswipl"
            ]
          }
        ]
      ]
    }
  ]
}