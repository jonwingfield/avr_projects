require 'email_notifier'

describe "Email Notification" do
	include Mail::Matchers

	context "when sending mail" do
		before(:each) do
			notifier = EmailNotifier.new(:to => "wingfield.jon@gmail.com")
			notifier.notify("This is a test email")
		end

		it { should have_sent_email.to("wingfield.jon@gmail.com") }
		it { should have_sent_email.with_subject('This is a test email') }
		it { should have_sent_email.from("weather_monitor@test.com")}
	end

	context "Warning emails" do
		before(:each) do
			notifier = EmailNotifier.new(:to => "wingfield.jon@gmail.com", :level => "WARNING")
			notifier.notify("This is a test email")			
		end

		it { should have_sent_email.with_subject('WARNING: This is a test email') }
	end
end