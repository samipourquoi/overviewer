import { Request, Response } from "express";
import { GetChunkRequest } from "requests";
import Core from "../core";

export module ChunksProvider {
    export function getChunk(req: Request, res: Response) {
        try {
            let request = req.query as unknown as GetChunkRequest;
            let x = +request.x;
            let z = +request.z;
            res.contentType("png")
                .end(Core.getOrReadChunkAt(x, z));
        } catch (e) {
            res.send(e);
        }
    }
}