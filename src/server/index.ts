import * as express from "express";
import routes from "./routes";
import * as ffi from "ffi";
import { CoreAPI } from "core";

export const core: CoreAPI = ffi.Library("build/core/libcore", {
    "core_start": [ "void", [] ],
    "core_end": [ "void", [] ]
});

core.core_start();

export const app = express();
const port = process.env.PORT || 8080;

app.use("/", routes);

app.listen(port, () => {
   console.log(`Listening to port ${port}`);
});

process.on("exit", () => {
    core.core_end();
});