import smtplib

smtp_host = "smtp.mailtrap.io"
smtp_port = 2525
user = "48306dbff532ee"
password = "45dec9ccb3e291"

sender = "unknown@securemail.com"
receiver = "target@gmail.com"
subject = "Urgent: Action Required Immediately"
body = """helloooo, I've an interesting information. Hedgehog's spikes are not spiky.....

Thank me later!!!

- hodgehog
"""

def compose_email(sender, recipient, subject, body):
    """Formats the email message."""
    return f"""From: {sender}
To: {recipient}
Subject: {subject}
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8

{body}
"""

# main implementation
message = compose_email(sender, receiver, subject, body)

try:
    with smtplib.SMTP(smtp_host, smtp_port) as server:
        server.starttls()
        server.login(user, password)
        server.sendmail(sender, receiver, message.encode("utf-8"))
    print("Email sent to Mailtrap! Check inbox.")

except Exception as e:
    print(f"Failed to send email: {e}")