document.addEventListener('DOMContentLoaded', function () {
    fetch('/delete')
        .then(response => response.json())
        .then(data => {
            const fileList = document.getElementById('file-list');
            if (data.files && data.files.length > 0) {
                data.files.forEach(file => {
                    const fileItem = document.createElement('div');
                    fileItem.className = 'file-item';
                    fileItem.innerHTML = `
                        <span>${file}</span>
                        <button class="delete-button" data-file="${file}">Delete</button>
                    `;
                    fileList.appendChild(fileItem);
                });

                document.querySelectorAll('.delete-button').forEach(button => {
                    button.addEventListener('click', function () {
                        const fileName = this.getAttribute('data-file');
                        fetch(`/delete/${fileName}`, {
                            method: 'DELETE',
                        })
                        .then(response => response.json())
                        .then(result => {
                            if (result.success) {
                                this.parentElement.remove();
                            } else {
                                alert('Failed to delete file');
                            }
                        });
                    });
                });
            } else {
                fileList.innerHTML = '<p>No files found.</p>';
            }
        })
        .catch(error => {
            console.error('Error fetching file list:', error);
        });
});
