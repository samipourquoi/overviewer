export interface CoreAPI {
    core_start: () => void;
    core_end: () => void;
    chunks_get_at: (x: number, z: number) => Buffer;
}