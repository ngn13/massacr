from os import environ
import requests as req
from lib import *

def http_handler(ip, port):
    # if not an HTTP(S) server skip
    if port != 80 and port != 443:
        return api.db_add(ip, port, {})

    # if its a possible HTTP(S) server send a request 
    # and gather simple data
    protocol = "http" if (port == 80) else "https" 
    try:
        res = req.get(f"{protocol}://{ip}", verify=False)
    except:
        return api.db_add(ip, port, {})

    server = res.headers["Server"]
    body = res.text

    # api.db_add adds ip and port data to 'results' database,
    # if ip already exists, it just appends the port 
    # it also adds any extra information you provide with 
    # the last argument
    api.db_add(ip, port, {
        "http_server": server,
        "http_body": body,
    })
    # instead of 'api.db_add' you can access 
    # the mongo driver with 'api.mongo' and directly modify
    # the database however you like

if __name__ == "__main__":
    mongo_url = environ["API_MONGO"] if "API_MONGO" in environ else "mongodb://localhost"
    password  = environ["API_PASSWORD"] if "API_PASSWORD" in environ else "default"
    host      = "127.0.0.1" if "API_USE_LOCAL" in environ and environ["API_USE_LOCAL"] == "1" else "0.0.0.0"
    port      = 5000

    # creating the API
    api = API(
        mongo_url, # MongoDB database URL
        password,  # API password 
        host,      # Address to listen on
        port,      # Port to listen on
    )

    # use our custom HTTP handler
    # if no hanlder is set, API will just store IP and port info 
    # on the 'results' database
    api.set_handler(http_handler)

    # start the API
    api.run()
