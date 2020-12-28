import * as express from "express";
import routes from "./routes";
import Core from "./core";
import * as dotenv from "dotenv";
dotenv.config();

export const app = express();
const port = process.env.PORT || 8080;

Core.init();

app.use("/", routes);

app.listen(port, () => {
   console.log(`Listening to port ${port}`);
});
