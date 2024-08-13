import cgi
import os
import sys

UPLOAD_DIR = 'resources/uploads'  # 파일을 저장할 디렉토리

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
            return "Status: 201 Created"
        else:
            return "Status: 400 Bad Request"
    else:
        return "Status: 400 Bad Request"

def handle_post_request():
    content_type = os.environ.get('CONTENT_TYPE', '')

    if content_type.startswith('application/x-www-form-urlencoded'):
        # Handle form data
        data = sys.stdin.read()  # Read the raw POST data from stdin
        # 타겟이 없는 경우 저장하지 않음
        target = os.environ.get('TARGET')
        if not target:
            return "Status: 400 Bad Request"
        # target이 생성할 수 있는 파일명이 아닌 경우 저장하지 않음(ex. 폴더명)
        if '/' in target or '\\' in target:
            return "Status: 400 Bad Request"
        # 저장할 파일이 이미 존재하는 경우 저장하지 않음
        if os.path.exists(os.path.join(UPLOAD_DIR, target)):
            return "Status: 409 Conflict"
        file_path = os.path.join(UPLOAD_DIR, target)
        with open(file_path, 'w') as file:
            file.write(data)
        response = "Status: 201 Created"

    elif content_type.startswith('application/json'):
        # Handle JSON data
        data = sys.stdin.read()  # Read the raw POST data from stdin
        # 타겟이 없는 경우 저장하지 않음
        target = os.environ.get('TARGET')
        if not target:
            return "Status: 400 Bad Request"
        # target이 생성할 수 있는 파일명이 아닌 경우 저장하지 않음(ex. 폴더명)
        if '/' in target or '\\' in target:
            return "Status: 400 Bad Request"
        # 저장할 파일이 이미 존재하는 경우 저장하지 않음
        if os.path.exists(os.path.join(UPLOAD_DIR, target)):
            return "Status: 409 Conflict"
        file_path = os.path.join(UPLOAD_DIR, target)
        with open(file_path, 'w') as file:
            file.write(data)
        response = "Status: 201 Created"

    elif content_type.startswith('text/plain'):
        # Handle plain text
        data = sys.stdin.read()  # Read the raw POST data from stdin
        # 타겟이 없는 경우 저장하지 않음
        target = os.environ.get('TARGET')
        if not target:
            return "Status: 400 Bad Request"
        # target이 생성할 수 있는 파일명이 아닌 경우 저장하지 않음(ex. 폴더명)
        if '/' in target or '\\' in target:
            return "Status: 400 Bad Request"
        # 저장할 파일이 이미 존재하는 경우 저장하지 않음
        if os.path.exists(os.path.join(UPLOAD_DIR, target)):
            return "Status: 409 Conflict"
        file_path = os.path.join(UPLOAD_DIR, target)
        with open(file_path, 'w') as file:
            file.write(data)
        response = "Status: 201 Created"

    elif content_type.startswith('multipart/form-data'):
        # Handle file upload
        response = save_uploaded_file('file', UPLOAD_DIR)

    else:
        response = "Status: 400 Bad Request"

    return response

if __name__ == '__main__':
    response = handle_post_request()
    print("Content-Type: text/plain")
    print(response)