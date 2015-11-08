from flask import Flask
from flask import request

app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello World!'

@app.route('/overview/<sceneid>', methods=['GET', 'POST'])
def overview(sceneid):
    if request.method == 'POST':
        for k, v in request.files.items():
            print(k)
            #v.save(k)
        return 'Post ppp'
    else:
        return 'Test overview %s' % sceneid

if __name__ == '__main__':
    #app.debug = True
    app.run()
