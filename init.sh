# This shell script runs when the machine starts, 
# before the user-installed packages are set up.
#
# Use it to install additional dependencies.
#
# Examples:
#
# sudo apt update && sudo apt install -y some-package
#
# pip install some-package --additional-arguments
#
# The logs are stored in /tmp/log;
# the error log is copied to Notebook files.

sudo apt update && sudo apt-get install -y mc vim htop
if [ -e KasperskyOS-Community-Edition_1.1.1.40_en.deb ]
then 
    echo KasperskyOS CE SDK installation file present, skip downloading
else 
    wget -O KasperskyOS-Community-Edition_1.1.1.40_en.deb https://products.s.kaspersky-labs.com/special/KasperskyOSCommunityEdition/1.1.1.40/multilanguage-1.1.1.40/3737323236397c44454c7c31/KasperskyOS-Community-Edition_1.1.1.40_en.deb?guid=8eb27d12a59d4eeb893c496aec189e78
fi    

sudo apt-get install -y ./KasperskyOS-Community-Edition_1.1.1.40_en.deb

if [ -e cyberimmune-systems-example-traffic-light-kos ]
then
    echo repo cloning shall be done just once, then all files will be stored
else    
    git clone https://github.com/cyberimmunity-edu/cyberimmune-systems-example-traffic-light-kos.git
fi
# then just restore executable flags for the shell scripts as they get reset after virtual machine restart
chmod +x cyberimmune-systems-example-traffic-light-kos/*.sh