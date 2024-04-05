# massacr 🩸 mass IP/port scanner toolkit
An extensible toolkit for scanning the internet for TCP ports using SYN packets. 
Consists of different tools and servers that interact which each other:
```
Scanner -> API -> Handler -> MongoDB -> Mongo-Express
```
- [Scanner](scanner/): SYN port scanner written in C, sends requests to the API with curl
- [API](api/lib): Simple web API written with Flask, provides data to threaded handler
- [Handler](api/main.py): A simple extensible Python function to process provided data. When its done processing, 
it saves the processed data to the MongoDB database. Default handler gathers extra information about HTTP(S) servers.
- [MongoDB](https://www.mongodb.com/what-is-mongodb): NoSQL database for storing all the data 
- [Mongo-Express](https://github.com/mongo-express/mongo-express): Web-based MongoDB admin interface to interact with the data

## Deploy
Since there are multiple components of massacr, easiest 
way to deploy is to use `docker-compose`, here is an example configuration:
```yml
version: "3"
services:
  scanner:
    image: ghcr.io/ngn13/massacr/scanner
    command: --url=http://api:5000 --limit=100
    depends_on:
      - api 

  api:
    image: ghcr.io/ngn13/massacr/api
    restart: unless-stopped
    environment:
      - API_MONGO=mongodb://mongo
    depends_on:
      - mongo

  mongo:
    image: mongo 
    volumes:
      - ./db:/data/db:rw

  interface:
    image: mongo-express
    depends_on:
      - mongo
    environment:
      - ME_CONFIG_MONGODB_URL=mongodb://mongo
    ports:
      - "127.0.0.1:8081:8081"
```
after deploying the containers, you can access the web interface at `http://localhost:8081`.

## Configuration
### Scanner 
You can list all the options with `--help`:
```
--no-color => Do not print colored output
--recvport => Source port for TCP packets
--timeout  => Timeout for receiver thread
--ports    => Ports to scan for
--limit    => Packets per second limit
--debug    => Enable debug output
--url      => API HTTP(S) URL
--password => API password
```

- Use the `--<option>=<value>` syntax to set options.
- For the `--ports` option, you can specify a single port, or you can specify ranges with `-` (`1-100`) and multiple ports with `,` (`80,443,1234`)
- Timeout is the time to wait after sending all the packets (in seconds), as the receiver thread may fell behind
- `--limit` is set to 20 by default, **which is pretty slow, so you should increase it.**

> [!CAUTION]
> Do not go overkill on the `--limit` option, you will most likely end up using all the bandwidth 
> and crash the entire network.

Defaults for all the options are:
```
no-color => false
recvport => 13421
timeout  => 10
ports    => common
limit    => 20
debug    => false
url      => http://localhost:5000 
password => default
```

### API/Handler
Default API has few different options:
- `API_MONGO`: MongoDB URL
- `API_PASSWORD`: Password for API access
- `API_USE_LOCAL`: Set to 1 if you want API to listen only on the local interface (127.0.0.1)

However you can modify these options and the handler itself by modifying [`api/main.py`](api/main.py).
If you know a bit of python you can easily write your own handler for your specific use case. All the code 
is well commented but feel free to create issues to ask questions if you are stuck.

## Resources 
Here are some different resources that I used during the development:
- [SYN scanning](https://nmap.org/book/synscan.html) (massacr does not exactly use SYN scan, it does not send RST packets)
- [libnet](https://github.com/libnet/libnet) (provides an easy way to build and send raw network packets)
