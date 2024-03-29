# libimu-private

OpenLoop Implementation for the KVH-1725 FOG IMU.  This library runs on Linux and macOS and parses the KVH datastream.

See the `imu_config` and `imu_test` tools in the [hyperloop](https://github.com/ParadigmHyperloop/hyperloop) repo 

<img width="1871" alt="image" src="https://user-images.githubusercontent.com/1410448/202876803-c29b8e49-74ce-497e-9be4-067dd44d54da.png">

# Build

Type `make` to make the `libimu.a` static library which handles opening, reading, checking, and closing the IMU device.

Type `make install` to place `libimu.a` into a distribution root at `./BUILD/usr/local/lib`
```
$ make && make install

$ ls ./BUILD/usr/local/lib
libimu.a
```

To build the testing and debugging tools (`./config` and `./test`) type `make config && make test`, or just `make debug`.

Use the `install-debug` target to install the debug tools into a distribution root

```
$ make debug && make install-debug

$ ls ./BUILD/usr/local/bin
config    test
```

You can use the `config` program to put your KVH IMU into config mode then issue it commands.  See [config.log](config.log).

You can use the `test` program to verify that your IMU is performing nominally.
