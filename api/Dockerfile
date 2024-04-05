FROM python 

WORKDIR /api
COPY lib    ./lib
COPY *.py   ./
COPY *.txt  ./

RUN pip install -r requirements.txt

ARG API_PASSWORD 
ENV API_PASSWORD $API_PASSWORD

ARG API_USE_LOCAL 
ENV API_USE_LOCAL $API_USE_LOCAL

ARG API_MONGO 
ENV API_MONGO $API_MONGO

ENTRYPOINT ["python", "/api/main.py"] 
