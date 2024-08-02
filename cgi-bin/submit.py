import cgi
import os

def save_uploaded_file(form_field, upload_dir):
    form = cgi.FieldStorage()
    if form_field in form:
        fileitem = form[form_field]
        if fileitem.file:
            filepath = os.path.join(upload_dir, fileitem.filename)
            with open(filepath, 'wb') as fout:
                while True:
                    chunk = fileitem.file.read(100000)
                    if not chunk:
                        break
                    fout.write(chunk)
            print("Status: 303 See Other")
            print("Location: /upload/cgi_post.html")
        else:
            print("Status: 400 Bad Request")
    else:
        print("Status: 400 Bad Request")

print("Content-Type: text/html; charset=utf-8")
upload_dir = "resources/uploads/"
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir)
save_uploaded_file('file', upload_dir)