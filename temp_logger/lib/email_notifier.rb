class EmailNotifier
	def initialize options
		@options = options
		@options[:from] ||= "weather_monitor@test.com"
		if @options.has_key? :level
			@options[:level] += ": "
		else
			@options[:level] = ""
		end
	end

	def notify subject
		message = Mail.new(@options)
		message.subject = "#{@options[:level]}#{subject}"
		message.deliver!
	end
	alias_method :call, :notify
end