import { sendControllerCommand } from '$lib/server/controller';
import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

export const POST: RequestHandler = async ({ fetch }) => {
	const result = await sendControllerCommand(fetch, 'reset');

	if (!result.ok) {
		return json({ error: result.error }, { status: result.status });
	}

	return json({ status: 'accepted' }, { status: result.status });
};
