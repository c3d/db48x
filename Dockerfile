FROM ubuntu:24.04
WORKDIR /
USER root

ARG DEBIAN_FRONTEND=noninteractive
ARG PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
ENV HOME /root
ENV TZ=America/Los_Angeles

RUN apt-get update -qq \
    && apt-get install -yqq apt-utils                                                                       \
    && apt-get install -yqq apt-transport-https                                                             \
    && apt-get install -yqq tzdata                                                                          \
    && ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime                                        \
    && dpkg-reconfigure --frontend noninteractive tzdata                                                    \
    && apt-get install -yqq locales                                                                         \
    && locale-gen en_US.UTF-8                                                                               \
    && apt-get install -yqq bash vim-tiny git curl wget pigz bc                                             \
                           rsync sudo build-essential autogen automake                                      \
                           libtool unzip psmisc qmake6 qt6-declarative-dev qt6-tools-dev qt6-multimedia-dev \
                           libfreetype-dev libfreetype6 libfreetype6-dev mesa-common-dev                    \
                           man tofrodos gcc-arm-none-eabi pkg-config libfreetype-dev  libarchive-zip-perl   \
    && apt-get clean                                                                                        \
    && rm -rf /var/lib/apt/lists/*                                                                          \
    && mkdir -p /root                                                                                       \
    && groupadd -g 666 db48x                                                                                \
    && useradd -u 666 -g db48x -m db48x                                                                     \
    && chsh -s /usr/bin/bash db48x                                                                          \
    && sed -i 's/%sudo/#%sudo/' /etc/sudoers                                                                \
    && echo '%sudo ALL=NOPASSWD: ALL' >> /etc/sudoers                                                       \
    && usermod -a -G sudo db48x

RUN su db48x -lc "cd /home/db48x ; git clone -b stable --recurse-submodules https://github.com/c3d/db48x"
RUN su db48x -lc "cd /home/db48x/db48x ;                                                                    \
                    qtchooser -install qt6 $(which qmake6) ;                                                \
                    sudo mv ~/.config/qtchooser/qt6.conf /usr/share/qtchooser/qt6.conf ;                    \
                    sudo mkdir -p /usr/lib/$(uname -p)-linux-gnu/qt-default/qtchooser ;                     \
                    sudo ln -n /usr/share/qtchooser/qt6.conf /usr/lib/$(uname -p)-linux-gnu/qt-default/qtchooser/default.conf ; \
                    make sim"

RUN chown -R db48x:db48x ~db48x
RUN su db48x -lc "cd /home/db48x/db48x/tools/ttf2font ; make CPPFLAGS=-I/usr/include/freetype2 LDFLAGS=-lfreetype"
RUN su db48x -lc "cd /home/db48x/db48x/tools/forcecrc32 ; make"
RUN su db48x -lc "cd /home/db48x/db48x ; make dm32 || true"
RUN su db48x -lc "cd /home/db48x/db48x ; make dm32"

CMD ["/bin/bash"]
