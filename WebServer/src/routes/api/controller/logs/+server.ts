import { fetchControllerLogs } from '$lib/server/controller';
import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

export const GET: RequestHandler = async ({ fetch }) => {
	try {
		const entries = await fetchControllerLogs(fetch);
		return json({ entries });
	} catch (error) {
		return json(
			{
				error: error instanceof Error ? error.message : 'Failed to fetch controller logs'
			},
			{ status: 503 }
		);
	}
};
