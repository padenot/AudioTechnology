# Portaudio

- Wiki : <http://www.portaudio.com/trac/wiki>. Includes instructions on how to
build for each operating system. You'll need a `C` compiler, and some libraries.
  - Linux → trivial
  - Windows → install MinGW \& Cygwin
  - MacOSX → install XCode, then `sudo port install portaudio`
  - basically the classic `./configure && make && sudo make install`
- Examples are in `test/`. General line to compile : `gcc -lrt -lasound
-lpthread -lportaudio input.c`. `asound` must be replaced by the library
used by your system. This produce a `a.out` binary.

# libsndfile

- Library to handle audio files IO : `libsndfile`, multiplatform, etc :
<http://www.mega-nerd.com/libsndfile/>. It can handle disk streaming and such,
and can handle a variety of formats, so it should be quite cool for the project.
  - Ubuntu : `sudo apt-get install libsndfile-dev`
  - MacOSX : `sudo port install libsndfile`

# git
- Ubuntu : `sudo apt-get install git`
- Mac : `sudo port install git`

Workflow :

1. Edit files in the repository
2. When you're done, `git add filename1 filename2`
3. Then `git commit -m"A useful message"`. At this point, `git` saved a snapshot
   of the code tree (means that if you screw up, you can go back to this
   snapshot).
4. Then `git pull origin master` to eventually get the file modification from
   other people. This will merge the files automatically. If there is a
   conflict, it will tell you, and you'll have to merge the file by looking a
   them. After the manual merge, do another `git add file_merged`, `git commit
   -m"manual merge"`.
5. Then `git push origin master` to send the changes to the `github.com`
   repository.
6. To get a library used for logging and other delicacies, do a `git submodule
   init` and a `git submodule update`, to get it.
