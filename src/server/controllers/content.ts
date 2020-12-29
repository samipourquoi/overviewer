import { Request, Response } from "express";
import { GetChunkRequest } from "requests";
import Core from "../core";

export module ChunksProvider {
    export function getChunk(req: Request, res: Response) {
        try {
            let request = req.query as unknown as GetChunkRequest;
            let x = +request.x;
            let z = +request.z;
            console.log(x, z, process.env.REGION_PATH);

            // Core.bindings.chunk_read_and_render(Buffer.from(process.env.REGION_PATH as string), x, z);
            res.contentType("png")
                .end(Core.getOrReadChunkAt(request.x, request.z));
        } catch (e) {
            res.send(e);
        }
    }
}