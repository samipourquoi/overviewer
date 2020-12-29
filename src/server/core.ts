import * as ffi from "ffi";
import * as ref from "ref";

export module Core {
    export let bindings: Core.Bindings;

    export interface Bindings {
        core_start: () => void;
        core_end: () => void;
        chunks_get_at: (x: number, z: number) => Buffer;
        chunk_read_and_render: (path: Buffer, x: number, z: number) => ProcessStatus;
    }

    export enum ProcessStatus {
        CHUNK_NO_EXIST
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

    export function getOrReadChunkAt(x: number, z: number): Readonly<Buffer> | null {
        let chunk = getChunkAt(x, z);
        if (chunk == null) {
            let status = bindings.chunk_read_and_render(Buffer.from(generateRegionPath(x, z)), x, z);
            if (status == ProcessStatus.CHUNK_NO_EXIST) {
                return null;
            }
            return getChunkAt(x, z) as Readonly<Buffer>;
        } else {
            return chunk;
        }
    }

    function generateRegionPath(chunkX: number, chunkZ: number): string {
        let regionX = Math.floor(chunkX/32);
        let regionZ = Math.floor(chunkZ/32);
        return `${process.env.REGION_PATH}/r.-${regionX}.${regionZ}.mca`;
    }
}

export default Core;