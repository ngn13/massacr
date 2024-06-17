import { get_collection } from "$lib/mongo.js";
const PAGESIZE = 5;

export async function load({ params, url }) {
  let page = parseInt(url.searchParams.get("page"));
  page = page ? page : 1;

  let search = url.searchParams.get("query");
  let queries = search ? search.split(" ") : [];
  let ipv4_list = [], port_list = [];

  queries.forEach(q => {
    let values = q.split(":")
    if(values.length != 2)
      return;

    if(values[0] === "ipv4")
      ipv4_list.push(values[1]);

    else if(values[0] === "port"){
      let port = parseInt(values[1]);
      if(port !== NaN || port > 0 || port < 65535)
        port_list.push(port);
    }
  });

  let collection;

  try {
    collection = await get_collection();
  } catch (error) {
    return {
      list: [],
      size: 0,
      page: 1,
      pages: 1,
      error: "Failed to access to the database",
    };
  }

  let query = { ipv4: { $exists: true } };

  if(ipv4_list.length != 0 && port_list.length == 0)
    query = { ipv4: { $in: ipv4_list } }

  else if(ipv4_list.length == 0 && port_list.length != 0)
    query = { ports: { $in: port_list } }

  else if(ipv4_list.length != 0 && port_list.length != 0)
    query = { ports: { $in: port_list }, ipv4: { $in: ipv4_list } }

  let ret = [], size = 0, cursor;

  cursor = await collection.find(query).sort({_id:-1});
  size = await collection.countDocuments(query);
  cursor = await cursor.limit(PAGESIZE).skip(PAGESIZE*(page-1));

  for await (const d of cursor) {
    ret.push({
      ipv4: d["ipv4"],
      ports: d["ports"],
    });
  }

  let pages = Math.ceil(size/PAGESIZE);
  pages = pages == 0 ? 1 : pages;

  return {
    list: ret,
    size: size,
    page: page,
    pages: pages,
  };
}
