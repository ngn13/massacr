import { MongoClient } from "mongodb";
import dotenv from "dotenv";
dotenv.config();

async function get_collection() {
  let error = "";
  const uri = process.env["MONGO"];
  const mongo = new MongoClient(uri, {
    serverSelectionTimeoutMS: 5000,
    connectTimeoutMS: 5000,
    socketTimeoutMS: 5000,
  });

  await mongo.connect((err) => {
    error = err;
  });

  if (error != "") throw new Error(error);

  const db = mongo.db("massacr");
  const data = db.collection("data");
  return data;
}

export { get_collection };
