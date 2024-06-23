https://github.com/microsoft/vscode-cmake-tools/issues/477
or
https://github.com/microsoft/vscode-cmake-tools/issues/717

Possible scripts/configure.sh:

```sh
#!/usr/bin/env bash

if [ ! -d build ]; then
    mkdir build
    cd build
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
fi
```

with in Dockerfile:

```dockerfile
COPY scripts/configure.sh /opt
```

with in devcontainer.json:

```json
    "onCreateCommand": "/opt/configure.sh"
```
