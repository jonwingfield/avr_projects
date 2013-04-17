class Wunderground_Logger
	Upload_Url = "http://rtupdate.wunderground.com/weatherstation/updateweatherstation.php"

	def initialize id, password
		@id, @password = id, password
	end

	def log reading, time
		uri = URI(Upload_Url)
		uri.query = URI.encode_www_form({
			:action => 'updateraw',
			:ID => @id,
			:PASSWORD => @password,
			:tempf => reading.temperature.f,
			:humidity => reading.humidity.round,
			:dewptf => reading.dewpoint.f,
			:dateutc => time.utc.to_s.gsub(' UTC', ''),
			:realtime => 1,
			:rtfreq => 10
		})

		begin
			res = Net::HTTP.get_response(uri)
			puts Time.now.to_s + ": " + res.body if res.is_a?(Net::HTTPSuccess)
			return res.body == "success"
		rescue
			$stderr.print "#{Time.now.to_s}: #{$!}\n"
			return false
		end
	end	
end

