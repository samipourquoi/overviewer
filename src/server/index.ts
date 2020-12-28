import * as express from "express";
import routes from "./routes";
import * as ffi from "ffi";
import { CoreAPI } from "core";
import * as ref from "ref";

export const core: CoreAPI = ffi.Library("build/core/libcore", {
    "core_start": [ "void", [] ],
    "core_end": [ "void", [] ],
    "chunks_get_at": [ "char*", [ "int", "int" ] ]
});

core.core_start();
let a = core.chunks_get_at(0, 0);
console.log(ref.readCString(a));

export const app = express();
const port = process.env.PORT || 8080;

app.use("/", routes);

app.listen(port, () => {
   console.log(`Listening to port ${port}`);
});

process.on("exit", () => {
    core.core_end();
});