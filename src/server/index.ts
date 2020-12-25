import * as express from "express";
import routes from "./routes";

export const app = express();
const port = process.env.PORT || 8080;

app.use("/", routes);

app.listen(port, () => {
   console.log(`Listening to port ${port}`);
});