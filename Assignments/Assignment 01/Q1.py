from flask import Flask, request, render_template_string

app = Flask(__name__)

# Simple HTML page with a form
HTML_PAGE = """
<!DOCTYPE html>
<html>
<head>
    <title>Simple HTTP Server</title>
</head>
<body>
    <h2>Enter Your subject</h2>
    <form method="POST" action="/submit">
        <label for="name">Subject Name:</label>
        <input type="text" id="name" name="name" required>
        <button type="submit">Submit</button>
    </form>
</body>
</html>
"""

@app.route('/')
def home():
    return render_template_string(HTML_PAGE)

@app.route('/submit', methods=['POST'])
def submit():
    name = request.form.get('name')
    return f"Your form is submitted successfully.\nYou are currently learning {name}! "

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
