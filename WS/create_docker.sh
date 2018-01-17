docker rm opt-deadline-ws-run
docker build -t opt-deadline-ws .
docker run --name opt-deadline-ws-run -p 8080:8080 opt-deadline-ws
