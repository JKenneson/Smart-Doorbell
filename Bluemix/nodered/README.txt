Mainflow.json is the only file to be concerned in this directory to get working.

With IBM Cloud/Bluemix, create a Node-RED project.
Within the Node-RED visual editor, copy the entire mainFlow.json text, and select import -> clipboard.
Paste the text into the clipboard and import the flow.

Some things to be concerned with that may require further setup.

Visual recognition node requires API key in order to operate. This can be set up within IBM Cloud by searching for the Visual Recognition service and linking it to the Node-RED project.

The email notification with need an account and password in order to send an email.

Cloudant DB will need permissions adjusted so the database can be read from an unknown user. This is found under Cloudant permissions.
