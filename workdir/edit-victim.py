import lief,sys

abi = "armeabi-v7a"
tail = "-arm"
if(len(sys.argv) > 1 and sys.argv[1] == "arm64-v8a"):
    abi = sys.argv[1]
    tail = "-arm64"

path = "../victim/build/intermediates/cmake/debug/obj/"+abi+"/victim"
elf = lief.parse(path)
elf.add_library("/data/local/tmp/libinject"+tail+".so")
elf.write("victim-patch"+tail)
print("patch success")