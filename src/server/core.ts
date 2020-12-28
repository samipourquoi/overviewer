import * as ffi from "ffi";
import * as ref from "ref";

export module Core {
    export let bindings: Core.Bindings;

    export interface Bindings {
        core_start: () => void;
        core_end: () => void;
        read_region_file: (path: Buffer, x: number, z: number) => void;
        chunks_get_at: (x: number, z: number) => Buffer;
    }

    export function init(): void {
         bindings = ffi.Library("build/core/libcore", {
            "core_start":       [ "void",   [] ],
            "core_end":         [ "void",   [] ],
            "read_region_file": [ "int",    [ "char*", "int", "int" ] ],
            "chunks_get_at":    [ "char*",  [ "int", "int" ] ]
        });

        bindings.core_start();
        process.on("exit", bindings.core_end);
    }

    export function getChunkAt(x: number, z: number): Readonly<Buffer> {
        let raw: Buffer = bindings.chunks_get_at(x, z);
        let length: number = ref.readUInt64LE(raw);
        return ref.reinterpret(raw, length, 8);
    }
}

export default Core;