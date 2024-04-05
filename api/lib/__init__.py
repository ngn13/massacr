"""

 #  massacr/api | mass internet/ipv4 scanner
 #  ========================================
 #  written by ngn (https://ngn.tf) (2024)

 #  This program is free software: you can redistribute it and/or modify
 #  it under the terms of the GNU General Public License as published by
 #  the Free Software Foundation, either version 3 of the License, or
 #  (at your option) any later version.

 #  This program is distributed in the hope that it will be useful,
 #  but WITHOUT ANY WARRANTY; without even the implied warranty of
 #  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #  GNU General Public License for more details.

 #  You should have received a copy of the GNU General Public License
 #  along with this program.  If not, see <https://www.gnu.org/licenses/>.

"""

from pymongo.mongo_client import MongoClient
from flask import Flask, request
from threading import Thread 
import logging as log

class API:
    def __init__(self, mongo_url: str, password: str, host: str, port: int) -> None:
        self.handler = None
        self.app = Flask(__name__)
        self.mongo = MongoClient(mongo_url)
        self.password = password
        self.host = host 
        self.port = port

        werkzeug_log = log.getLogger("werkzeug")
        werkzeug_log.setLevel(log.ERROR)
        log.basicConfig(level=log.INFO, format="%(asctime)s [%(levelname)s]: %(message)s")

        self.app.add_url_rule("/scanner/check", "/scanner/check", self.route_check)
        self.app.add_url_rule("/scanner/add", "/scanner/add", self.route_add)

    def db_add(self, ip: str, port: int, extra):
        db = self.mongo["results"]
        col = db["hosts"]
        data = {
            "ipv4": ip,
            "ports": [port],
            "data": [extra] if len(extra.keys()) != 0 else [],
        }

        found = col.find_one({"ipv4":  ip})
        if not found:
            return col.insert_one(data)
       
        if not port in found["ports"]:
            col.update_one({"ipv4": ip}, {"$push": {"ports": port}})
            if len(extra.keys()) != 0:
                col.update_one({"ipv4": ip}, {"$push": {"data": extra}})

    def set_handler(self, func):
        self.handler = func

    def run(self):
        log.info("Trying to connect to mongodb")
        try:
            log.info(f"Connected to mongodb version {self.mongo.server_info()['version']}")
        except Exception as e:
            log.critical(f"Database connection failed: {e}")
            exit(1)

        log.info(f"Starting API server on {self.host}:{self.port}")
        self.app.run(host=self.host, port=self.port)

    def route_check(self):
        password = request.args["pass"]
        if self.password != password:
            return "Bad password", 403
        
        log.info(f"Password check successful for {request.remote_addr}")
        return "OK", 200

    def route_add(self):
        password = request.args["pass"]
        port = request.args["port"]
        ip = request.args["ip"]

        if self.password != password:
            return "Bad password", 403

        if self.handler == None:
            self.db_add(ip, int(port), {})

        t = Thread(target=self.handler, args=(ip,int(port),))
        t.daemon = True
        t.run()

        return "OK", 200
