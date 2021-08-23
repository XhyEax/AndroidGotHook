import lief

path = "../victim/build/intermediates/cmake/debug/obj/armeabi-v7a/victim"
elf = lief.parse(path)
elf.add_library("/data/local/tmp/libinject.so")
elf.write("victim-patch")
print("patch success")