librd - Rapid Development C library
===================================

Copyright (c) 2012-2013, [Magnus Edenhill](http://www.edenhill.se/), et.al.

[https://github.com/edenhill/librd](https://github.com/edenhill/librd)

**librd** aims to provide the commonly needed helpers, sub systems, snippets
and misc. functionality that lacks from the standard C libraries, allowing for
rapid development of new programs and **non-intrusive** extension of existing
applications.

**librd** is **non-intrusive** in the sense that single specific functionality
from **librd** can be used by the application without having to use or
initialize other parts of the library. In its most simple form you add
`-lrd -lz -lrt` to your linking step and include the proper `rd<FUNC>.h`
include file for your desired functionality.

**librd** is licensed under the 2-clause BSD license.



**librd** provides, on a higher level:

- proper thread support throughout the library
- consistent, natural and non-bloated APIs and interfaces
- scalability and performance
- proper documentation (some time in the future)
- suitable for embedded systems, large-scale backend systems, GUI applications,
  etc.


# Functionality

Non-exhaustive list of current **librd** functionality:

- `rdqueue.h`: Thread-safe FIFO queues (nice for worker queues).
- `rdthread.h`: Thread management abstraction.
- `rdmem.h`: Memory contexts for contextual malloc's allowing memory
     usage supervision and free-all-context-memory-at-once.
- `rdmem.h`: Efficient memory and allocation helpers: `rd_calloc_
- `rdsysqueue.h`: Improved sys/queue.h
- `rdopt.h`: Short (-c) and long (--config) command line argument option
    parsing with input validation and automatic variable assignments.
- `rdfloat.h`: Float comparison helpers.
- `rdaddr.h`: `AF_INET` and `AF_INET6` agnostification.
- `rdbuf.h`: Generic buffers with (de)serializer/writer/reader callbacks.
- `rdstring.h`: String helpers: `rd_strnchrs()`.
- `rd.h`: Convenience macros and porting alleviation:
   `RD_CAP*(), RD_ARRAY_SIZE(), RD_ARRAY_ELEM(), RD_MIN(), RD_MAX()`.
- `rdavl.h`: Thread-safe AVL trees.
- `rdio.h`: Socket/fd IO abstraction and helpers.
- `rdfile.h`: File/filesystem access helpers.
- `rdencoding.h`: Various encoder and decoder helpers (varint).


# Usage

## Requirements
	The GNU toolchain
   	pthreads
	zlib

## Instructions

### Building

      make all
      make install
      # or to install in another location than /usr/local:
      DESTDIR=/my/prefix make install

### Rigid building

The above procedure continues on test failures, but that might not be
desirable, so here's how to do make sure the tests pass before installing.

      make libs test install


      
### Usage in code

      #include <librd/rd.h>
      #include <librd/rdFUNCTIONALITY.h>

      ...
      rd_init();
      ...

      rd_....();

Link your program with `-lrd -lz -lrt`.


## Documentation

Documentation is still lacking, but each public function and concept is
described in its `.h` header file while internal functions are described
in their `.c` files.


## Testing

Regression tests are integrated into the build process.
The tests are in the `tests/` sub-directory.


## Examples

The test programs in the `tests/` sub-directory serve as examples, but some
more explicit examples will be provided in the `examples/` sub-directory.

More documentation and usage examples will come.


# Apache Kafka

**NOTE:** The Kafka implementation has been moved to its own library.
See [librdkafka](https://github.com/edenhill/librdkafka).


# Public projects using librd

-  [Image Judge](https://github.com/edenhill/imagejudge) (TBA)



# TODO

Scattered lists of things to do:

- Generic thread-safe linked-lists that does not require a struct field.
  i.e.: `obj->ll = rd_ll_add(mylist, obj);`
- **librd** should be aware of the number of CPU cores so it can be used
  to create an appropriate number of workers, if thats what matters.
  i.e.: `rd_workers_create(..., rd_conf_getu32("sys.cpu.cores") * 16);`
- Configuration/.rc file support using the rd_opt_t framework allowing
   the same configuration token to be specified both as command line
   argument, CLI command and configuration file directive without any
   extra development effort.
- rdmkbuildtree.sh - script to generate makefiles at the start of a project,
  debian directories, etc.
- Add anything else that may come in handy for more than one program ever.

