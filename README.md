# VIM--Visual_studio--API
This repository gives VIM(classic) access to *most of Visual Studio extensions, add ons', etc and the ability 
to make API calls from VIM through Visual Studio. I'm aware that this is not intented. I need to use VIM for 
my disibility and cannot use Visual studio. I'm also aware that this is not perfect code.

When I say Vim, I mean VIM classic 1991. This is also neoVim compatable, but please reconsider your life
choices if you use neoVim



----------Attempt 1----------------------
how to use this, first download the attempt 1 folder, secondly copy paste "Bash_commands" into your linux terminal or Windows powershell, thirdly add VimPackage to your VIM folder, and finally make the script live by executiving "chmod +x vim-api-sender.sh" in your linux terminal or windows powershell,=.

an example of how to prompt this repo in VIM is :% write !curl -X POST https://api.endpoint.com -H "Content
-Type: application/json" -d @-"



----------Attempt 2----------------------
First, downoad the attempt 2 folder , secondly put the LEGALLY ACQUIRED api key into a enviroment before you open vim like so(export API_KEY="YOUR_LEGAL_API_KEY_GOES_HERE"), then put the vim


This is "serverless" but designed to connect VIM to a server. This is an add-on for VIM, which this repo
is trying to make connect to the internet thourgh API calls, but this code is "self-contained" and requires
hardware to run(i.e. open RAM space and a CPU thread, for a moment).
