# libimu-private

OpenLoop IMU Implementation


# Build

```
$ make && make install

$ ls ./BUILD/usr/local/lib
libimu.a
```

To build the testing and debugging tools

```
$ make debug && make install-debug

$ ls ./BUILD/usr/local/bin
config    test
```

You can use the `config` program to put your KVH IMU into config mode then issue it commands.  See [config.log](config.log).

# License

Closed Source, Copyright 2016 Eddie Hurtig and The OpenLoop Alliance

DO NOT REDISTRIBUTE
