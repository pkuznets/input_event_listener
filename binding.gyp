{
  "target_defaults":
    {
        "cflags" : ["-Wall", "-Wextra"],
        "include_dirs": ["<!(node -e \"require('..')\")"]
    },
  "targets": [
    {
        "target_name" : "input_event_listener"
      , "sources"     : [ "src/input_event_listener.cpp" ]
    }
]}
