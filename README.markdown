librd - Rapid Development C library
===================================

Copyright (c) 2012, [Magnus Edenhill](http://www.edenhill.se/), et.al.

[https://github.com/edenhill/librd](https://github.com/edenhill/librd)

**librd** aims to provide the commonly needed helpers, sub systems, snippets
and misc. functionality that lacks from the standard C libraries, allowing for
rapid development of new programs and non-intrusive extension of existing
applications.

**librd** is licensed under the 2-clause BSD license.



**librd** provides, on a higher level:

- proper thread support throughout the library
- consistent, natural and non-bloated APIs and interfaces
- scalability and performance
- proper documentation (some time in the future)
- suitable both for embedded systems as well as large-scale backend systems


# Functionality

Non-exhaustive list of current **librd** functionality:

- Thread-safe FIFO queues (nice for worker queues).
- Thread management abstraction.
- Memory contexts for contextual malloc's allowing memory usage supervision
  and free-all-context-memory-at-once.
- Improved sys/queue.h.
- Short (-c) and long (--config) command line argument option parsing with
  input validation and automatic variable assignments.
- Float comparison helpers.

# Usage

## Requirements
	The GNU toolchain
   	pthreads

## Instructions

### Building

      make
      make install  <-- FIXME, not added

### Usage in code

      #include <rd.h>
      #include <rdFUNCTIONALITY.h>

      ...
      rd_init();
      ...

      rd_....();

Link your program with `-lrd`.




## Examples

More documentation and usage examples will come.
For now, check the .c files in the tests/ directory for usage examples.



# Projects using librd

-  [Image Judge](https://github.com/edenhill/imagejudge)



# TODO

Scattered lists of things to do:

- Generic thread-safe linked-lists that does not require a struct field.
  i.e.: `obj->ll = rd_ll_add(mylist, obj);`
- **librd** should be aware of the number of CPU cores so it can be used
  to create an appropriate number of workers, if thats what matters.
  i.e.: `rd_workers_create(..., rd_conf_getu32("sys.cpu.cores") * 16);`
- Configuration/.rc file support using the rd_opt_t framework allowing
   the same configuration token to be specified both as command line
   argument and configuration file without any extra development effort.
- Add anything else that may come in handy for more than one program ever.

