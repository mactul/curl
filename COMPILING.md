To link with quiceh, you have to link both curl and quiceh with boringssl.  
To do so, the ideal is to compile boringssl in a separate directory

```sh
git clone https://boringssl.googlesource.com/boringssl
cd boringssl
mkdir build
cd build
cmake -DCMAKE_POSITION_INDEPENDENT_CODE=on -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release ..
make -j8
cd ..
mkdir lib
cp build/*.so lib/ # don't use sym link, rust does not like that at all but you will not have any error message, it will just don't work at runtime

pwd
# copy this path, we will refer to it later as <BORINGSSL_REPO_PATH>
```

The go the the quiceh repository and run:
```sh
QUICHE_BSSL_PATH=<BORINGSSL_REPO_PATH> QUICHE_BSSL_LINK_KIND=dylib cargo build --lib --examples --features ffi,pkg-config-meta --release

pwd
# copy this path, we will refer to it later as <QUICEH_REPO_PATH>
```

Finally, in this repo, run:
```sh
autoreconf -fi
./configure --with-openssl=<BORINGSSL_REPO_PATH> --with-quiceh=<QUICEH_REPO_PATH>/target/release/
make -j8
```

Finally, to run curl, run:
```
LD_LIBRARY_PATH=<QUICEH_REPO_PATH>/target/release/:<BORINGSSL_REPO_PATH> ./src/curl --http3-only -k https://127.0.0.1:4433/README.md
```