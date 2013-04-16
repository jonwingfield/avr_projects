class Temperature
	attr_reader :c, :f

	def initialize(temp)
		if temp.instance_of? String
			if temp.end_with? 'F'
				@f = temp.to_f
				@c = (5.0/9.0 * temp - 32).round(1)
				return
			end
		end
		temp = temp.to_f
		@c = temp
		@f = (9.0/5.0 * temp + 32).round(1)
	end

	def self.from(thing)
		if thing.instance_of? Temperature
			thing
		else
			Temperature.new(thing)
		end
	end

	def >(temp)
		@c > temp.c
	end

	def <(temp)
		@c < temp.c
	end

	def to_s
		"#{@c}C"
	end
end


class WeatherReading
	def self.from_sensor(data)
		values = data.split ','
		hashed = {}
		
		values.each do |val|
			kv = val.split ':'
			hashed[kv[0].strip] = kv[1].strip
		end

		return WeatherReading.new(hashed["Temperature"].to_f, hashed["Humidity"].to_f)		
	end

	attr_reader :temperature, :humidity, :dewpoint

	def initialize(temp, humidity)
		@temperature = Temperature.new(temp)
		@humidity = humidity
		@dewpoint = Temperature.new(calculate_dewpoint(temp, humidity))
	end


private
	
	def calculate_dewpoint(temp, humidity)
		b = 18.678
		c = 257.14
		gamma = Math.log(humidity / 100) + (b * temp) / (c + temp)
		dewpoint = (c * gamma) / (b - gamma)
		dewpoint.round(1)
	end
end

