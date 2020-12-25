import * as express from "express";
import routes from "./routes";
import * as ffi from "ffi";

export const core = ffi.Library("build/core/libcore.dylib", {});
export const app = express();
const port = process.env.PORT || 8080;

app.use("/", routes);

app.listen(port, () => {
   console.log(`Listening to port ${port}`);
});