import { Router } from "express";
import { ChunksProvider } from "../controllers/content";

const router = Router();

router.get("/", ChunksProvider.getChunk);

export default router;