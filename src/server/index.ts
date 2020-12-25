import * as express from "express";
import routes from "./routes";
import * as ffi from "ffi";
import { CoreAPI } from "../types/core";

export const core: CoreAPI = ffi.Library("build/core/libcore.dylib", {
    "assets_init": [ "void", [] ]
});
core.assets_init();

export const app = express();
const port = process.env.PORT || 8080;

app.use("/", routes);

app.listen(port, () => {
   console.log(`Listening to port ${port}`);
});