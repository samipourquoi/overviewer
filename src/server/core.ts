import * as ffi from "ffi";
import * as ref from "ref";
import { NULL_POINTER } from "ref";

export module Core {
    export let bindings: Core.Bindings;

    export interface Bindings {
        core_start: () => void;
        core_end: () => void;
        chunks_get_at: (x: number, z: number) => Buffer;
        chunk_read_and_render: (path: Buffer, x: number, z: number) => Buffer;
    }

    export function init(): void {
         bindings = ffi.Library("build/core/libcore", {
            "core_start":       [ "void",   [] ],
            "core_end":         [ "void",   [] ],
            "chunks_get_at":    [ "char*",  [ "int", "int" ] ],
            "chunk_read_and_render": [ "int", [ "char*", "int", "int" ] ]
        });

        bindings.core_start();
        process.on("exit", bindings.core_end);
    }

    export function getChunkAt(x: number, z: number): Readonly<Buffer> | null {
        let raw: Buffer = bindings.chunks_get_at(x, z);
        if (ref.isNull(raw)) return null;
        let length: number = ref.readUInt64LE(raw);
        return ref.reinterpret(raw, length, 8);
    }

    export function getOrReadChunkAt(x: number, z: number): Readonly<Buffer> {
        let chunk = getChunkAt(x, z);
        if (chunk == null) {
            bindings.chunk_read_and_render(Buffer.from(process.env.REGION_PATH as string), x, z);
            return getChunkAt(x, z) as Readonly<Buffer>;
        } else {
            return chunk;
        }
    }
}

export default Core;