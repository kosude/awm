# awm: Multithreaded development branch

This branch contains experimental code for running awm on multiple threads. I don't currently plan on ever merging this, since it doesn't really work,
and when it does, it is usually the same speed or even slower (since the same data is being accessed bv different threads when handling events so a
lot of memory is locked most of the time). I'm really just keeping this here in case I ever do come back to improve it, or if anyone has a better idea
of how multitheading could be implemented. (Maybe plugins could be run on different threads?)
