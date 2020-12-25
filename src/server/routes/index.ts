import * as express from "express";
import chunks from "./chunks";

const router = express.Router();

router.use("/", express.static("src/public"));
router.use("/chunks", chunks);

export default router;