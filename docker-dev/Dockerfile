FROM node:6

RUN npm install -g bs-platform
RUN apt-get update && apt-get install tmux

USER node

WORKDIR /home/node
ENV HOME=/home/node
CMD ["bash"]
