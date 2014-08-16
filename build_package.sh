echo "- removing /tmp/my-install"
sudo rm -rf /tmp/my-install
ninja install
echo "- running macdeployqt"
sudo /usr/local/Cellar/qt/4.8.4/bin/macdeployqt /tmp/my-install/bin/QtClassroom.app
